(:*******************************************************:)
(: Test: K2-ConDocNode-25                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure text nodes gets merged(#2).           :)
(:*******************************************************:)
count(document{
text {"data"},
text {"data"},
text {"data"},
text {"data"}
}/child::node())