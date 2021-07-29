(:*******************************************************:)
(: Test: K2-SeqDocFunc-12                                :)
(: Written by: Frans Englich                             :)
(: Date: 2007-11-22T11:31:21+01:00                       :)
(: Purpose: General query complexity, combined with a call to fn:doc() that is never evaluated. :)
(:*******************************************************:)
declare variable $fileToOpen := <Variable id="_7" name="constComplex2" type="_11c" context="_1" location="f0:17" file="f0" line="17"/>;
  empty($fileToOpen//*[let $i := @type
  return doc($fileToOpen)//*[$i]])