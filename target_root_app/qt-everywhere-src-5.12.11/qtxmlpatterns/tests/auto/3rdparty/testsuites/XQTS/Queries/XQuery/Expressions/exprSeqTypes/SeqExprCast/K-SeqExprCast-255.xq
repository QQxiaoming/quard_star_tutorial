(:*******************************************************:)
(: Test: K-SeqExprCast-255                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure casting xs:gDay to xs:string, with timezone 'Z' is properly handled. :)
(:*******************************************************:)
xs:string(xs:gDay("---01Z")) eq "---01Z"