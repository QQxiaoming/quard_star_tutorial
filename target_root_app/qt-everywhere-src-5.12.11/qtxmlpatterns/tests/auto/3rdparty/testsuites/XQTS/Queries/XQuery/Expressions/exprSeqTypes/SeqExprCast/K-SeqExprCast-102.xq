(:*******************************************************:)
(: Test: K-SeqExprCast-102                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:hexBinary("aa")) eq "AA"`. :)
(:*******************************************************:)
xs:string(xs:hexBinary("aa")) eq "AA"