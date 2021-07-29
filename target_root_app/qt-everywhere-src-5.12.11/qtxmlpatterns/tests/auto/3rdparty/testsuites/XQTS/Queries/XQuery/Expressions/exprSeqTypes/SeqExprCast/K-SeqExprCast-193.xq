(:*******************************************************:)
(: Test: K-SeqExprCast-193                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The canonical lexical representation for the xs:dayTimeDuration value P3D is 'P3D'. :)
(:*******************************************************:)
xs:string(xs:dayTimeDuration("P3D")) eq "P3D"