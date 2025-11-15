import os
from posixpath import basename
import regex
import sys

def askForFolderOrDefault(inputstr, default):
    """
    Ask the user for a folder path. If '' is entered, use the default.
    Ensures the folder exists and is a directory.
    """
    if(len(sys.argv) == 1):
        protoheaderfoldername = os.path.normpath(input(inputstr).strip())
    else:
        protoheaderfoldername = '.'
    
    # Use default if user enters ''
    if protoheaderfoldername == '.':
        protoheaderfoldername = os.path.normpath(default)
        print("using deafult yay:", protoheaderfoldername)

    # Validate folder existence
    if not os.path.exists(protoheaderfoldername) or not os.path.isdir(protoheaderfoldername):
        print("you are gone.")
        exit(123)

    return protoheaderfoldername


def askForFileOrDefault(inputstr, default):
    """
    Ask the user for a file path. If '' is entered, use the default.
    Ensures the file exists and is a file.
    """
    if(len(sys.argv) == 1):
        protoheaderfilename = os.path.normpath(input(inputstr).strip())
    else:
        protoheaderfilename = '.'

    # Use default if user enters ''
    if protoheaderfilename == '.':
        protoheaderfilename = os.path.normpath(default)
        print("using deafult yay:", protoheaderfilename)

    # Validate file existence
    # if not os.path.exists(protoheaderfilename) or not os.path.isfile(protoheaderfilename):
    #     print("you are gone.")
    #     exit(123)

    return protoheaderfilename



def main():
    # -------------------------------------------------------------------
    # IO paths
    # -------------------------------------------------------------------
    protoheaderfoldername = askForFolderOrDefault(
        "whar the dfoklder of ther proto headers???????? ",
        "../_C++"
    )
    protoheaderfiles = os.listdir(protoheaderfoldername)

    outputsourcefile = askForFileOrDefault(
        "whar is OUTPUT SOURCE FILE??????????????? WILL BE DESTROYED!!!!!!!!!!!!!!!!!!!!!!",
        "../Components/StructToProto.cpp"
    )
    outputheaderfile = askForFileOrDefault(
        "whar is OUTPUT HEADER FILE??????????????? WILL BE DESTROYED!!!!!!!!!!!!!!!!!!!!!!",
        "../Components/StructToProto.hpp"
    )
    outputheaderfileformatted = os.path.basename(outputheaderfile).replace(".hpp", "")

    # -------------------------------------------------------------------
    # Source output buf
    # -------------------------------------------------------------------
    outbuf = (
        "/* \n*\n*\n*\n*\n*\n*\n*\n"
        "this file is auto  gen erated!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! DO NOT TOICH!!!!!*/\n"
        "#include <%s>\n\n" % os.path.basename(outputheaderfile)
    )

    # -------------------------------------------------------------------
    # regex patterns
    # -------------------------------------------------------------------
    dataclassreg = regex.compile(
        r"class [a-zA-Z][a-zA-Z\d]* final\: public \:\:EmbeddedProto\:\:MessageInterface\n"
    )
    fieldnamefinderreg = regex.compile(
        r"static constexpr char const\* ([A-Z_]+)_NAME = \"(\w+)\";"
    )
    typefinderreg = regex.compile(
        r"inline const (\w+)& get_(\w+)\(\) const { return \w*_*\.?\w+_(.get\(\))*; }"
    )

    # -------------------------------------------------------------------
    # Generate header file
    # -------------------------------------------------------------------
    with open(outputheaderfile, "wt+") as out:
        out.truncate()
        out.seek(0)

        # Header guard
        out.write("#ifndef INC_%s_HPP_\n#define INC_%s_HPP_\n\n" %
                  (outputheaderfileformatted.upper(), outputheaderfileformatted.upper()))

        # Include all proto headers
        for protoheaderfile in protoheaderfiles:
            out.write("#include <%s>\n" % protoheaderfile)

        # Template declaration
        out.write(
            "\ntemplate<typename T, typename = std::enable_if_t<std::is_base_of<EmbeddedProto::MessageInterface, T>::value>>\n"
        )
        out.write("T StructToProtoMsg(const void* data);\n")

        # End of header guard
        out.write("\n#endif /* INC_%s_HPP_ */" % outputheaderfileformatted.upper())


    # -------------------------------------------------------------------
    # Generate source file
    # -------------------------------------------------------------------
    with open(outputsourcefile, "wt+") as out:
        foundClassNames = []

        # Process each proto header file
        for protoheaderfile in protoheaderfiles:
            with open(os.path.join(protoheaderfoldername, protoheaderfile), "r") as f:
                print("=======\n", protoheaderfile,"\n=======")
                buf = f.read()

                # Find all class definitions
                dataclasses = dataclassreg.finditer(buf)

                for dataclass in dataclasses:
                    classname = dataclass.group().split()[1]

                    # Keep track of proto class names so we dont specify "Proto::" for builtins later
                    foundClassNames.append(classname)

                    # -------------------------------
                    # FINDING CLASS DEFINITION BOUNDS
                    # -------------------------------

                    bracketstack = []
                    pos = dataclass.span()[0]
                    firstboundingbracketpos = 0
                    lastboundingbracketpos = 0
                    stringType = None

                    while pos < len(buf):
                        thischar = buf[pos]

                        # Opening brace
                        if thischar == '{' and stringType is None:
                            bracketstack.append(pos)
                            if firstboundingbracketpos == 0:
                                firstboundingbracketpos = pos

                        # Closing brace
                        elif thischar == '}' and stringType is None:
                            if len(bracketstack) <= 1:
                                lastboundingbracketpos = pos
                                break
                            else:
                                bracketstack.pop()

                        # Enter and exit string literals with matching quotes
                        elif thischar == "\"":
                            if buf[pos - 1] != "\\":
                                if stringType == "\"":
                                    stringType = None
                                elif stringType is None:
                                    stringType = "\""
                        elif thischar == "'":
                            if buf[pos - 1] != "\\":
                                if stringType == "'":
                                    stringType = None
                                elif stringType is None:
                                    stringType = "'"

                        pos += 1

                    # Extract field names
                    fielddefs = fieldnamefinderreg.finditer(
                        buf[firstboundingbracketpos:lastboundingbracketpos]
                    )
                    fieldnames = [fn.group(2) for fn in fielddefs]
                    print("%s fields:" % classname, ", ".join(fieldnames))

                    # Generate template specialization for this class
                    outbuf += (
                        "template<> Proto::%s StructToProtoMsg<Proto::%s>(const void* data) {\n"
                        % (classname, classname)
                    )
                    outbuf += "\tconst uint8_t* curr = (const uint8_t*)data;\n"
                    outbuf += "\tProto::%s result;\n\n" % classname

                    # Find types of fields
                    types = typefinderreg.finditer(
                        buf[firstboundingbracketpos:lastboundingbracketpos]
                    )
                    typeDict = {i.group(2): i.group(1) for i in types}

                    # Generate field assignments
                    for fn in fieldnames:
                        if typeDict[fn] in foundClassNames:
                            outbuf += "\tresult.set_%s(*(Proto::%s*)curr);\n" % (fn, typeDict[fn])
                            outbuf += "\tcurr += sizeof(Proto::%s);\n" % typeDict[fn]
                        else:
                            outbuf += "\tresult.set_%s(*(decltype(result.%s())*)curr);\n" % (fn, fn)
                            outbuf += "\tcurr += sizeof(decltype(result.%s()));\n" % fn

                    outbuf += "\n\treturn result;\n};\n\n\n"

        # Write final buffer to source file
        out.truncate()
        out.seek(0)
        out.write(outbuf)

if(__name__ == "__main__"):
    main()