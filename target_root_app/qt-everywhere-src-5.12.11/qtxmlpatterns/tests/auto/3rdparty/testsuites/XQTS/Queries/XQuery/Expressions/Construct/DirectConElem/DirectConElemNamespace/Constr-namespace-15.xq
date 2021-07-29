(: Name: Constr-namespace-15 :)
(: Written by: Andreas Behm :)
(: Description: Namespace Declarations - use undeclared prolog prefix in child :)

declare namespace foo="http://www.example.com/prolog";
<elem xmlns:foo=""><foo:child/></elem>