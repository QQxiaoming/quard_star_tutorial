(:*******************************************************:)
(:Test: year-from-date-17                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "year-from-date" function       :)
(:as part of a "numeric-equal" expression (ne operator)  :) 
(:*******************************************************:)

fn:year-from-date(xs:date("1970-01-01Z")) ne fn:year-from-date(xs:date("1970-01-01Z"))
