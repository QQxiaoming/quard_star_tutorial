(: Name: Constr-namespace- :)
(: Written by: Andreas Behm :)
(: Description: Namespace Declarations - select child of element overriding namespace prefix :)

(<foo:elem xmlns:foo="http://www.example.com/parent"><child xmlns:foo=""><grand-child/></child></foo:elem>)//grand-child