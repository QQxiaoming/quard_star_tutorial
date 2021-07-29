(: Name: fn-element-with-id-4 :)
(: Description: Contrast behaviour of id() vs element-with-id(). :)
(: Element of with type list of xs:ID - recognized only when the value is a singleton :)

(: insert-start :)
import schema namespace ids="http://www.w3.org/XQueryTest/ididrefs";
declare variable $input-context1 external;
(: insert-end :)

<out xmlns="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:i="http://www.w3.org/XQueryTest/ididrefs"
     xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
  <e>{$input-context1/id('xi')}</e>
  <f>{$input-context1/element-with-id('xi')}</f>
  <e>{$input-context1/id('ping')}</e>
  <f>{$input-context1/element-with-id('ping')}</f>  
</out>

