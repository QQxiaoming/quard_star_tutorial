(: Name: Constr-compelem-baseuri-2 :)
(: Written by: Andreas Behm :)
(: Description: base-uri through parent :)

fn:base-uri(exactly-one((<elem xml:base="http://www.example.com">{element a {}}</elem>)/a))
