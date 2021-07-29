(: Name: Constr-attr-nsdecl-1 :)
(: Written by: Andreas Behm :)
(: Description: namespace declaration does not count as attribute :)

fn:count((<elem xmlns:foo="http://ns.example.com/uri"/>)/@*)