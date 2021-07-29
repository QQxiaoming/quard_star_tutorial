(:*******************************************************:)
(: Test: K2-NameTest-17                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A name test matching only attributes.        :)
(:*******************************************************:)
declare variable $e := <a b ="content"><?b asd?><b/></a>;
              <a>{$e/@b}</a>