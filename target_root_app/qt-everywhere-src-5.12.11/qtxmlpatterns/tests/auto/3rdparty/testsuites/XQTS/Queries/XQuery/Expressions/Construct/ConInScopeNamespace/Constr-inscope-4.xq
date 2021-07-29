(: Name: Constr-inscope-4 :)
(: Written by: Andreas Behm :)
(: Description: Rename inscope namespace :)

for $x in <inscope>
<parent1 xmlns:foo="http://www.example.com/parent1" foo:attr1="attr1"/>
<parent2 xmlns:foo="http://www.example.com/parent2" foo:attr2="attr2"/></inscope>
return <new>{$x//@*:attr1, $x//@*:attr2}</new>