(:*******************************************************:)
(: Test: K-SeqExprCast-127                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:base64Binary("aaa a")) eq "aaaa"`. :)
(:*******************************************************:)
xs:string(xs:base64Binary("aaa a")) eq "aaaa"