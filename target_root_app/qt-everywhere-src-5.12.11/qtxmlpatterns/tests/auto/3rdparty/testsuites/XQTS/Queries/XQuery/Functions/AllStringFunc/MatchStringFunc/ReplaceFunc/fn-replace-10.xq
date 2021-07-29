(: Name: fn-replace-10:)
(: Description: Two alternatives within the pattern both match at the same position in the $input.  The first one is chosen. :)

(: insert-start :)
declare variable $input-context1 external;
(: insert-end :)

fn:replace("abcd", "(ab)|(a)", "[1=$1][2=$2]")