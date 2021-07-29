(:*******************************************************:)
(: Test: K2-ConDocNode-18                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure processing instructions and comments are ignored when extracting the string value from a document node. :)
(:*******************************************************:)
string(document{
        text {"data"},
        processing-instruction name {"data"},
        processing-instruction name {"data"},
        text {"data"},
        processing-instruction name {"data"},
        processing-instruction name1 {"data"},
        comment {"content"},
        comment {"content"},
        text {"data"},
        processing-instruction name2 {"data"},
        comment {"content"},
        text {"data"}
        })