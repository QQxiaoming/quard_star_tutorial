(:*******************************************************:)
(: Test: K-SeqExprCast-270                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure casting xs:gMonth to xs:string, with timezone 'Z' is properly handled. :)
(:*******************************************************:)
xs:string(xs:gMonth("--01Z")) eq "--01Z"