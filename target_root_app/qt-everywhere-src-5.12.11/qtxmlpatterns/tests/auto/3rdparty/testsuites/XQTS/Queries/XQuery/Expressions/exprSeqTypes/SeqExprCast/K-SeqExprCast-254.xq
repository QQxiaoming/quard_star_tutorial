(:*******************************************************:)
(: Test: K-SeqExprCast-254                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure casting xs:gDay to xs:string, with timezone '+00:00' is properly handled. :)
(:*******************************************************:)
xs:string(xs:gDay("---01+00:00")) eq "---01Z"