(:*******************************************************:)
(:Test: year-from-date-16                                :)
(:Written By: Carmelo Montanez                           :)
(:Date: June 6, 2005                                     :)
(:Purpose: Evaluates The "year-from-date" function       :)
(:as part of a "numeric-equal" expression (eq operator)  :) 
(:*******************************************************:)

fn:year-from-date(xs:date("1970-01-01Z")) eq fn:year-from-date(xs:date("1970-01-01Z"))
