(: Name: fn-element-with-id-1 :)
(: Description: Contrast behaviour of id() vs element-with-id(). :)
(: Element of type derived from xs:ID by restriction :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

<out xmlns="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:i="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <e>{$input-context1/id('lambda mu')}</e>
  <f>{$input-context1/element-with-id('lambda mu')}</f>
</out>

