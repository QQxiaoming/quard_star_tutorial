(:*******************************************************:)
(: Test: K-SeqExprCast-107                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:hexBinary("fA")) eq "FA"`. :)
(:*******************************************************:)
xs:string(xs:hexBinary("fA")) eq "FA"