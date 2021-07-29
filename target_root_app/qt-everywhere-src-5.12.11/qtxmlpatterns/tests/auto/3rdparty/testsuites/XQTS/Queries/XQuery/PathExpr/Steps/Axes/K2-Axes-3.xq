(:*******************************************************:)
(: Test: K2-Axes-3                                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Path expression where the last step in an xs:integer. :)
(:*******************************************************:)
declare variable $v := <a><b/><b/><b/></a>;
deep-equal($v//45, (45, 45, 45, 45))