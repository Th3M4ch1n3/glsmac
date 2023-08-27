#include "ChoiceList.h"

namespace ui {
namespace object {

ChoiceList::ChoiceList( const std::string& class_name ) : UIContainer( class_name ) {
	SetEventContexts( EC_KEYBOARD );
	
	// TODO: move to styles
	m_item_align.margin = 2;
	m_item_align.height = 20;
}

void ChoiceList::SetImmediateMode( const bool immediate_mode ) {
	if ( m_immediate_mode != immediate_mode ) {
		ASSERT( m_buttons.empty(), "can't change mode after initialization" );
		m_immediate_mode = immediate_mode;
	}
}

void ChoiceList::SetChoices( const choices_t& choices ) {
	ASSERT( m_choices.empty(), "choices already set" );
	
	m_choices = choices;
	
	if ( m_created ) {
		UpdateButtons();
	}
}

void ChoiceList::SetValue( const std::string& value ) {
	auto it = m_buttons.find( value );
	ASSERT( it != m_buttons.end(), "value does not exist in choices" );
	SetActiveButton( it->second );
}

const std::string& ChoiceList::GetValue() const {
	ASSERT( !m_choices.empty(), "choices are empty" );
	ASSERT( m_value < m_choices.size(), "choices value overflow" );
	ASSERT( m_value >= 0, "choices value not set" );
	return m_choices[ m_value ];
}

void ChoiceList::Create() {
	UIContainer::Create();
	
	if ( m_buttons.empty() && !m_choices.empty() ) {
		UpdateButtons();
	}
	
}

void ChoiceList::Destroy() {
	
	for ( auto& button : m_buttons ) {
		RemoveChild( button.second );
	}
	m_buttons.clear();
	
	UIContainer::Destroy();
}

void ChoiceList::Align() {
	UIContainer::Align();
	
	if ( !m_buttons.empty() ) {
		size_t value = 0;
		for ( auto& choice : m_choices ) {
			auto* button = m_buttons.at( choice );
			button->SetHeight( m_item_align.height );
			button->SetTop( m_item_align.margin + ( m_item_align.height + m_item_align.margin ) * (value) );
			value++;
		}
	}
}

/*void ChoiceList::OnChange( UIEventHandler::handler_function_t func ) {
	On( UIEvent::EV_CHANGE, func );
}*/


void ChoiceList::SetItemMargin( const coord_t item_margin ) {
	m_item_align.margin = item_margin;
	Realign();
}

void ChoiceList::SetItemHeight( const coord_t item_height ) {
	m_item_align.height = item_height;
	Realign();
}

void ChoiceList::ApplyStyle() {
	UIContainer::ApplyStyle();
	
	if ( Has( Style::A_ITEM_MARGIN ) ) {
		SetItemMargin( Get( Style::A_ITEM_MARGIN ) );
	}
	
	if ( Has( Style::A_ITEM_HEIGHT ) ) {
		SetItemHeight( Get( Style::A_ITEM_HEIGHT ) );
	}
}

void ChoiceList::UpdateButtons() {
	if ( m_created ) {
		for ( auto& button : m_buttons ) {
			RemoveChild( button.second );
		}
		m_buttons.clear();
		size_t value = 0;
		for ( auto& choice : m_choices ) {
			NEWV( button, Button );
				button->SetLabel( choice );
				button->SetAlign( ALIGN_TOP );
				button->SetTextAlign( ALIGN_LEFT | ALIGN_VCENTER );
				button->SetLeft( 3 );
				button->SetRight( 3 );
				button->ForwardStyleAttributesV( m_forwarded_style_attributes );
				button->On( UIEvent::EV_BUTTON_CLICK, EH( this, button ) {
					if ( !button->HasStyleModifier( Style::M_SELECTED ) ) {
						SetActiveButton( button );
					}
					if ( m_immediate_mode ) {
						SelectChoice();
					}
					return true;
				});
				button->On( UIEvent::EV_BUTTON_DOUBLE_CLICK, EH( this ) {
					if ( !m_immediate_mode ) {
						SelectChoice();
					}
					return true;
				});
			AddChild( button );
			m_button_values[ button ] = value;
			m_buttons[ choice ] = button;
			value++;
		}
		Realign();
		if ( !m_immediate_mode ) {
			SetValue( m_choices[ 0 ] ); // activate first by default
		}
	}
}

bool ChoiceList::OnKeyDown( const UIEvent::event_data_t* data ) {
	switch ( data->key.code ) {
		case UIEvent::K_DOWN: {
			if ( m_value < m_choices.size() - 2 ) {
				SetValue( m_choices[ m_value + 1 ] );
			}
			else {
				SetValue( m_choices.back() );
			}
			break;
		}
		case UIEvent::K_UP: {
			if ( m_value > 0 ) {
				SetValue( m_choices[ m_value - 1 ] );
			}
			else {
				SetValue( m_choices[ 0 ] );
			}
			break;
		}
		case UIEvent::K_ENTER: {
			SelectChoice();
			break;
		}
		default: {
			return true;
		}
	}
	return true;
}

bool ChoiceList::OnKeyUp( const UIEvent::event_data_t* data ) {
	//Log( "KEY UP" );
	return true;
}

bool ChoiceList::OnKeyPress( const UIEvent::event_data_t* data ) {
	//Log( "KEY PRESS" );
	return true;
}

void ChoiceList::SetActiveButton( Button* button ) {
	auto it = m_button_values.find( button );
	ASSERT( it != m_button_values.end(), "button not in buttons list" );
	for ( auto& b : m_buttons ) {
		if ( b.second != button && b.second->HasStyleModifier( Style::M_SELECTED ) ) {
			b.second->RemoveStyleModifier( Style::M_SELECTED );
		}
	}
	if ( !button->HasStyleModifier( Style::M_SELECTED ) ) {
		button->AddStyleModifier( Style::M_SELECTED );
	}
	ASSERT( it->second < m_choices.size(), "button value overflow" );
	m_value = it->second;
}

void ChoiceList::SelectChoice() {
	if ( m_value >= 0 ) {
		UIEvent::event_data_t d = {};
		d.value.text.ptr = &GetValue();
		Trigger( UIEvent::EV_SELECT, &d );
	}
}

}
}
