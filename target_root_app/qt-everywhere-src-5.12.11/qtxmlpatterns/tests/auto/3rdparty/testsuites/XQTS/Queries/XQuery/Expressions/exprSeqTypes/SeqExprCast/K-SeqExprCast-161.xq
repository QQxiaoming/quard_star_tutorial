(:*******************************************************:)
(: Test: K-SeqExprCast-161                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The canonical lexical representation for the xs:duration value P31D is 'P31D'. :)
(:*******************************************************:)
xs:string(xs:duration("P31D")) eq "P31D"