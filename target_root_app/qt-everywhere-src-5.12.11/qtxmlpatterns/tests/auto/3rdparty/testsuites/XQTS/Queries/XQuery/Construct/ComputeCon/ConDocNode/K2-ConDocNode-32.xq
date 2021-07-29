(:*******************************************************:)
(: Test: K2-ConDocNode-32                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure a text node is properly constructed with nested document constructors, when serializing. :)
(:*******************************************************:)
document {1, document {2, document {document {()}, 3, document {4}}, 5}, 6}