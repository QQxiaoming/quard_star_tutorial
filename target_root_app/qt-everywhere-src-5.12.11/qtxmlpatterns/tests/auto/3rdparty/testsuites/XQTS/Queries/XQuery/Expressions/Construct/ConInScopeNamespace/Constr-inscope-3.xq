(: Name: Constr-inscope-3 :)
(: Written by: Andreas Behm :)
(: Description: Rename inscope namespace :)

for $x in <parent1 xmlns:foo="http://www.example.com/parent1" foo:attr1="attr1"/>
return <new xmlns:foo="http://www.example.com">{$x//@*:attr1}</new>