(:*******************************************************:)
(: Test: K-SeqExprCast-140                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:hexBinary(xs:base64Binary("Ow=="))) eq "3B"`. :)
(:*******************************************************:)
xs:string(xs:hexBinary(xs:base64Binary("Ow=="))) eq "3B"