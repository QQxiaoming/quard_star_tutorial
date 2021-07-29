(: Name: Constr-inscope-12 :)
(: Written by: Andreas Behm :)
(: Description: copy element node with namespace :)

for $x in <parent2 xmlns:foo="http://www.example.com/parent2" foo:attr2="attr2"><child2 attr="child"/></parent2>
return <new xmlns="http://www.example.com">{$x//*:child2}</new>