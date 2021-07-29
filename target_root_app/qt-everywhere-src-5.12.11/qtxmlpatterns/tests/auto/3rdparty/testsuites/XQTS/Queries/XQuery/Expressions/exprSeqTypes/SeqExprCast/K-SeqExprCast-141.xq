(:*******************************************************:)
(: Test: K-SeqExprCast-141                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:hexBinary(xs:base64Binary("aaa a"))) eq "69A69A"`. :)
(:*******************************************************:)
xs:string(xs:hexBinary(xs:base64Binary("aaa a"))) eq "69A69A"