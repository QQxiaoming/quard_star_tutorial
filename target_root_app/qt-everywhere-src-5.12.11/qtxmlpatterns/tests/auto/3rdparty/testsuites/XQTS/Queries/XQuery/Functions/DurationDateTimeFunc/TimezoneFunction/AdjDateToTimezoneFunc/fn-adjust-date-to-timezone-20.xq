(:*******************************************************:)
(:Test: adjust-date-to-timezone-20                       :)
(:Written By: Carmelo Montanez                           :)
(:Date: August 10, 2005                                  :)
(:Test Description: Evaluates The "adjust-date-to-timezone" function :)
(:as part of a comparisson expression (ge operator).     :)
(:*******************************************************:)

fn:adjust-date-to-timezone(xs:date("2002-03-07-04:00")) ge fn:adjust-date-to-timezone(xs:date("2005-03-07-04:00"))