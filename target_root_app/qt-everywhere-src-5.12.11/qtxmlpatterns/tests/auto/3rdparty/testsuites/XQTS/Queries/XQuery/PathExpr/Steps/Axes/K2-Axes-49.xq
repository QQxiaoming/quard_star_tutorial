(:*******************************************************:)
(: Test: K2-Axes-49                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply '/' to a variable, with fn:number() at the end. That is, an implicit dependency on the next-last step. :)
(:*******************************************************:)
declare variable $myVar := <e/>;
$myVar/(<a/>, <b/>, <?d ?>, <!-- e-->, attribute name {}, document {()})/number()