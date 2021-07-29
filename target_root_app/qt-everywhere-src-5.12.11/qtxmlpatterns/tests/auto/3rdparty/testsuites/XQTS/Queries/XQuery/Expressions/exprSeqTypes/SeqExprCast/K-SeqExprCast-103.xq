(:*******************************************************:)
(: Test: K-SeqExprCast-103                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:hexBinary("0a")) eq "0A"`. :)
(:*******************************************************:)
xs:string(xs:hexBinary("0a")) eq "0A"