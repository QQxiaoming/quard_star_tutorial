(:*******************************************************:)
(: Test: K2-NameTest-58                                  :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: A 'prefix:*' as operand to 'eq'.             :)
(:*******************************************************:)
 declare namespace prefix = "http://example.com/";
                declare variable $a := <e><a/><b/><c/></e>;
              <a>{$a/prefix:* eq 1}</a>