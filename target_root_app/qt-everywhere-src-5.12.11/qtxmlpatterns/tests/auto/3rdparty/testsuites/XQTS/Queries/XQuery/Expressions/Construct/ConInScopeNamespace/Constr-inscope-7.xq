(: Name: Constr-inscope-7 :)
(: Written by: Andreas Behm :)
(: Description: copy element node with namespace :)

for $x in <parent3 xmlns:foo="http://www.example.com/parent3"><foo:child3/></parent3>
return <new>{$x//*:child3}</new>