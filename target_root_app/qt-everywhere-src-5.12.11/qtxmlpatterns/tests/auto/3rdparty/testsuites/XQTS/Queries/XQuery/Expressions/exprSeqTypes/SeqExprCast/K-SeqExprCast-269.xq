(:*******************************************************:)
(: Test: K-SeqExprCast-269                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure casting xs:gMonth to xs:string, with timezone '+00:00' is properly handled. :)
(:*******************************************************:)
xs:string(xs:gMonth("--01+00:00")) eq "--01Z"