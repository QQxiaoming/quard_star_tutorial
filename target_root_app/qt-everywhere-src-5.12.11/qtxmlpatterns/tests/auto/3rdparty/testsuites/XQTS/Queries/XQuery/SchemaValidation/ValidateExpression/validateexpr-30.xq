(: Name : validateexpr-30 :)
(: Description: Test that validating a document node (with processing instructions and comments) does not throw a type error in static typing implementations when used as an expression of type document(). :)
(: Author: Oliver Hallam :)
(: Date: 2010-03-15 :)

(: insert-start :)
import schema default element namespace "http://www.w3.org/XQueryTestOrderBy";
(: insert-end :)

let $document as document-node(element(Strings)) := 
  document { (
    <!--comment-->, 
    <Strings><orderData>one string</orderData></Strings>,
    <?processing instruction?>
  ) }
let $validated as document-node(schema-element(Strings)) :=
  validate { $document }
return count($validated/node())
