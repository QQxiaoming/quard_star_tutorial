(: Name: Constr-namespace-12 :)
(: Written by: Andreas Behm :)
(: Description: Namespace Declarations - select child of element overriding prolog namespace prefix :)

declare namespace foo="http://www.example.com/prolog";
(<elem xmlns:foo="http://www.example.com/parent"><foo:child/></elem>)//*:child