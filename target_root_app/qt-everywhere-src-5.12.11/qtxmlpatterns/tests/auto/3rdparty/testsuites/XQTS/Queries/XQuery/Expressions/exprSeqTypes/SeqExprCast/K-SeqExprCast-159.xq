(:*******************************************************:)
(: Test: K-SeqExprCast-159                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: The canonical lexical representation for the xs:duration value P365D is 'P365D'. :)
(:*******************************************************:)
xs:string(xs:duration("P365D")) eq "P365D"