(:*******************************************************:)
(: Test: K2-DirectConElemContent-48                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-10-03T14:53:32+01:00                       :)
(: Purpose: Serialize an undeclaration.                  :)
(:*******************************************************:)
<e xmlns="http://www.example.com/">
        <a xmlns=""/>
</e>,
<e xmlns="http://www.example.com/">
        <a xmlns=""/>
    </e>/count(in-scope-prefixes(exactly-one(a))),

<e xmlns="http://www.example.com/">
    <a xmlns="">
        <b xmlns=""/>
    </a>
</e>
