xquery version "1.1";
(:*******************************************************:)
(: Test: switch-903                                      :)
(: Written by: Michael Kay                               :)
(: Purpose: switch, static error, no cases               :)
(:*******************************************************:)

declare variable $in external := 2;

<out>{
switch ($in) 
   default return "Baa"
}</out>    