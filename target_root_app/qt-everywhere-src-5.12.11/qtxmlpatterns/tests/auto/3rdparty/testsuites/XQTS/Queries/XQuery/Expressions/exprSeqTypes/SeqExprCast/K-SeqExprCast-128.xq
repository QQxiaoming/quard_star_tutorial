(:*******************************************************:)
(: Test: K-SeqExprCast-128                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A test whose essence is: `xs:string(xs:base64Binary("frfhforlksid7453")) eq "frfhforlksid7453"`. :)
(:*******************************************************:)
xs:string(xs:base64Binary("frfhforlksid7453")) eq "frfhforlksid7453"