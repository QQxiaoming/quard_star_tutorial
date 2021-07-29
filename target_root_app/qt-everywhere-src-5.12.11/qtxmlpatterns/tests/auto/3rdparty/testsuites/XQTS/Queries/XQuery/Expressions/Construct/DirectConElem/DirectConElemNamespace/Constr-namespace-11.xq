(: Name: Constr-namespace-11 :)
(: Written by: Andreas Behm :)
(: Description: Namespace Declarations - select child of element overriding parent namespace prefix :)

(<elem xmlns:foo="http://www.example.com/parent"><foo:child xmlns:foo="http://www.example.com/child"><foo:grand-child/></foo:child></elem>)//*:grand-child