(:*******************************************************:)
(: Test: K2-DirectConElemNamespace-43                    :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A direct element constructor with a redundant namespace declaration. :)
(:*******************************************************:)
<e xmlns="http://www.example.com/">
    <e xmlns="http://www.example.com/"/>
</e>