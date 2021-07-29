(:*******************************************************:)
(: Test: K-SeqExprCast-110                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:hexBinary("03")) eq "03"`. :)
(:*******************************************************:)
xs:string(xs:hexBinary("03")) eq "03"