(:*******************************************************:)
(: Test: K2-ExternalVariablesWith-12                     :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Variable declarations doesn't cause type conversion. :)
(:*******************************************************:)
declare variable $i as xs:integer := xs:untypedAtomic("1");
$i