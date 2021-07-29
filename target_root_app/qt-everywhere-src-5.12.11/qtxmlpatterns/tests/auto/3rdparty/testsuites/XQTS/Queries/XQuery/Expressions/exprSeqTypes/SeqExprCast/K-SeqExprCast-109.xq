(:*******************************************************:)
(: Test: K-SeqExprCast-109                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:hexBinary("031a34123b")) eq "031A34123B"`. :)
(:*******************************************************:)
xs:string(xs:hexBinary("031a34123b")) eq "031A34123B"