(:*******************************************************:)
(: Test: context-lib.xq                                  :)
(: Written By: Mary Holstege                             :)
(: Date: 2005/12/05 14:47:00                             :)
(: Purpose: Library module with interesting context      :)
(:*******************************************************:)

module namespace ctx="http://www.w3.org/TestModules/context";

(: insert-start :)

import schema namespace simple="http://www.w3.org/XQueryTest/simple";

(: insert-end :)

declare namespace unknown="http://www.w3.org/TestModules/unknown";

declare function ctx:use-schema ()
{
   "47" cast as simple:myType
};

declare function ctx:use-namespace ()
{
   fn:data(<unknown:thing>47</unknown:thing>) 
};