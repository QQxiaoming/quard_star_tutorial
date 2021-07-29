(:*******************************************************:)
(: Test: K-SeqExprCast-175                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that when casting xs:dayTimeDuration to xs:string, that empty fields are properly serialized. :)
(:*******************************************************:)
xs:string(xs:dayTimeDuration("P0DT00H00M00.000S")) eq "PT0S"