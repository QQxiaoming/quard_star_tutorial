(:*******************************************************:)
(: Test: K-SeqExprCast-403                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure when casting xs:date to xs:string that an empty milli seconds part is not serialized. :)
(:*******************************************************:)
xs:string(xs:date("2002-02-15")) eq "2002-02-15"