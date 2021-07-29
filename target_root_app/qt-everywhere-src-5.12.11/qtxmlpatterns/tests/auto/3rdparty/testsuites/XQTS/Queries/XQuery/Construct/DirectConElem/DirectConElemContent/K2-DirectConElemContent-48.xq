(:*******************************************************:)
(: Test: K2-DirectConElemContent-48                      :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: Serialize an undeclaration.                  :)
(:*******************************************************:)
<e xmlns="http://www.example.com/">
        <a xmlns=""/>
</e>,
<e xmlns="http://www.example.com/">
        <a xmlns=""/>
    </e>/count(in-scope-prefixes(a)),

<e xmlns="http://www.example.com/">
    <a xmlns="">
        <b xmlns=""/>
    </a>
</e>