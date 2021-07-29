(: Name: Constr-cont-nsmode-11 :)
(: Description: Test copying of potentially namespace-sensitive nodes preserves their type correctly. :)
(: Author: Oliver Hallam :)
(: Date: 2010-02-24 :)

declare construction preserve;
declare copy-namespaces no-preserve,inherit;
import schema namespace ns="http://www.example.com/test/namespace-sensitive";

declare variable $input-context external;

let $node := <e xmlns:bar="http://www.example.org/bar">{$input-context//ns:insensitive}</e>
return data($node//ns:insensitive) instance of ns:myString
