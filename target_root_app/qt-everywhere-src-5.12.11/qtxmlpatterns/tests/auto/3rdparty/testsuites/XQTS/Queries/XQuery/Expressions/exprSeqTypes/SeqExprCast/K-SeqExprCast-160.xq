(:*******************************************************:)
(: Test: K-SeqExprCast-160                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The canonical lexical representation for the xs:duration value P12M is 'P1Y'. :)
(:*******************************************************:)
xs:string(xs:duration("P12M")) eq "P1Y"