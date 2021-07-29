(:*******************************************************:)
(: Test: K2-ConDocNode-24                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure text nodes gets merged.               :)
(:*******************************************************:)
count(document{
text {"data"},
text {"data"},
<e/>,
text {"data"},
text {"data"}
}/child::node())