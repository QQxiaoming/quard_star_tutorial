(:*******************************************************:)
(: Test: K-SeqExprCast-108                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:hexBinary("10")) eq "10"`. :)
(:*******************************************************:)
xs:string(xs:hexBinary("10")) eq "10"