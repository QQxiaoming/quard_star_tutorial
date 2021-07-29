(:*******************************************************:)
(: Test: K2-ConDocNode-23                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Extract the string value from a document node with four computed text nodes. :)
(:*******************************************************:)
string(document{
        text {"data"},
        text {"data"},
        text {"data"},
        text {"data"}
})