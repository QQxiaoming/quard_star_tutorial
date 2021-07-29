(: Name: fn-element-with-id-1 :)
(: Description: Contrast behaviour of id() vs element-with-id(). :)
(: tests Erratum FO.E31 - new element-with-id() function :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

<out xmlns="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:i="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <e>{$input-context1/id('alpha beta iota kappa')}</e>
  <f>{$input-context1/element-with-id('alpha beta iota kappa')}</f>
</out>

