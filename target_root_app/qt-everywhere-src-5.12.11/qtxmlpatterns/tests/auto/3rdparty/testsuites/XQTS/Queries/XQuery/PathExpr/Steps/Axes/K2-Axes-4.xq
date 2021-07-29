(:*******************************************************:)
(: Test: K2-Axes-4                                       :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Path expression where the last step in an xs:integer(#2). :)
(:*******************************************************:)
declare variable $v := <a><b/><b/><b/></a>;
$v//45