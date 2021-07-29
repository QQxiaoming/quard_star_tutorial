(:*******************************************************:)
(: Test: K-SeqExprCast-157                               :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Ensure that when casting xs:duration to xs:string, that empty components are handled properly. :)
(:*******************************************************:)
xs:string(xs:duration("-PT8H23M0S")) eq "-PT8H23M"