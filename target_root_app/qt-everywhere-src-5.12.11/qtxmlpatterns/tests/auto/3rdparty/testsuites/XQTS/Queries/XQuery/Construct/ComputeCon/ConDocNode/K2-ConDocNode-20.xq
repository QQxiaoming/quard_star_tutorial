(:*******************************************************:)
(: Test: K2-ConDocNode-20                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A recursive construction of document nodes, combined with the comma operator. :)
(:*******************************************************:)
1, document{document{document{document{()}}}}/child::node(), 1