(: Name: fn-element-with-id-5 :)
(: Description: Contrast behaviour of id() vs element-with-id(). :)
(: Element of with type union of xs:ID  :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

<out xmlns="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:i="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <e>{$input-context1/id('omicron')}</e>
  <f>{$input-context1/element-with-id('omicron')}</f>
  <e>{$input-context1/id('853')}</e>
  <f>{$input-context1/element-with-id('853')}</f>  
</out>

