(:*******************************************************:)
(: Test: K2-Axes-48                                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Apply '/' to a variable, with an xs:integer at the end. :)
(:*******************************************************:)
declare variable $myVar := <e/>;
$myVar/(<a/>, <b/>, <?d ?>, <!-- e-->, attribute name {}, document {()})/3