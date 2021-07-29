(:*******************************************************:)
(: Test: K2-ConDocNode-12                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Extract the string value from a document node. :)
(:*******************************************************:)
<a>{string(document{<a/>, <b/>, <c/>})}</a>