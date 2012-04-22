<?php

function feedback_addsubj_form($form, &$form_state) {
	$form['subj'] = array(
		'#type' => 'textfield',
		'#title' => 'Название темы',
		'#size' => 40,
		'#maxlength' => 45,
		'#required' => TRUE,
	);

	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Добавить',
	);
	return $form;
}

function feedback_addsubj_form_validate($form, &$form_state) {
	if($form_state['values']['subj'] == ""){
		form_set_error('subj', 'Название темы не должно быть пустым');
	}
}

function feedback_addsubj_form_submit($form, &$form_state) {
	db_insert('feedback_subj')->fields(array("subject" => $form_state['values']['subj']))->execute();
	$form_state['redirect'] = 'admin/feedback/subj';
}



function feedback_subj_list(){
	$header = array(
		'subj' => array('data' => 'Тема', 'field' => 'f.subject'),
		'options' => array('data' => 'Действия'),
	);

	$query = db_select('feedback_subj', 'f');

	$count_query = clone $query;
	$count_query->addExpression('COUNT(f.fsid)');

	$query = $query->extend('PagerDefault')->extend('TableSort');
	$query->fields('f')
			->limit(10)
			->orderByHeader($header)
			->setCountQuery($count_query);
	$result = $query->execute();

	$rows = array();
	foreach ($result as $item) {
		$del = l('Удалить', "admin/feedback/delete-subj/$item->fsid");
		$rows[] = array(
			'class' => array('feedback-row-' . $item->fsid),
			'data' => array(
				$item->subject,
				$del
			),
		);
	}

	if($result->rowCount()){
		$rows[] = array(
			'data' => array(
				array(
					'data' => theme('pager'),
					'colspan' => 2,
				),
			),
		);
	}

	$element = array(
		'#theme' => 'table',
		'#rows' => $rows,
		'#header' => $header,
		'#empty' => 'Нет записей',
		'#attributes' => array('class' => array('feedback-table'))
	);

	return $element;
}

function feedback_list(){
	$header = array(
		'fio' => array('data' => 'ФИО', 'field' => 'f.fio'),
		'created' => array('data' => 'Отправлено', 'field' => 'f.created', 'sort' => 'desc'),
		'mail' => array('data' => 'Mail'),
		'subject' => array('data' => 'Тема'),
		'mesage' => array('data' => 'Сообшение'),
		'file' => array('data' => 'Файл'),
		'options' => array('data' => 'Действия'),
	);

	$query = db_select('feedback', 'f');



	$count_query = clone $query;
	$count_query->addExpression('COUNT(f.fid)');

	$query = $query->extend('PagerDefault')->extend('TableSort');
	$query->fields('f')
			->limit(20)
			->orderByHeader($header)
			->setCountQuery($count_query);


$query->addField("fs", "subject");
	$query->addJoin("inner", 'feedback_subj', "fs", 'f.subject = fs.fsid');
	$result = $query->execute();

	$rows = array();
	foreach ($result as $item) {
		$del = l('Удалить', "admin/feedback/delete/$item->fid");
		$file = '';
		if (!is_null($item->file)) {
			$file = file_load($item->file);
			$file = l('Скачать', "admin/feedback/$item->fid/download");
		}
		$rows[] = array(
			'class' => array('feedback-row-' . $item->fid),
			'data' => array(
				$item->fio,
				format_date($item->created),
				$item->mail,
				$item->subject,
				$item->message,
				$file,
				$del
			),
		);
	}

	if($result->rowCount()){
		$rows[] = array(
			'data' => array(
				array(
					'data' => theme('pager'),
					'colspan' => 7,
				),
			),
		);
	}

	$element = array(
		'#theme' => 'table',
		'#rows' => $rows,
		'#header' => $header,
		'#empty' => 'Нет записей',
		'#attributes' => array('class' => array('feedback-table'))
	);

	return $element;
}

function feedback_delete($fid){
    $form = drupal_get_form('feedback_delete_form', $fid);
    return drupal_render($form);
}

function feedback_delete_form($form, &$form_state, $fid) {
	$form = array();
	$form['fid'] = array(
		'#type' => 'hidden',
		'#value' => $fid,
	);
	$confirm = confirm_form($form, t('Вы действительно хотите удалить эту запись ?'), 'admin/feedback', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
	return $confirm;
}

function feedback_delete_form_submit($form, &$form_state) {
	$form = array();
	$fid = $form_state['values']['fid'];
	
	$file = db_query("SELECT file FROM {feedback} 
				WHERE fid = :fid ", array(':fid' => $fid))->fetchField();
	if ($file) {
		$file = file_load($fid);
		file_delete($file, TRUE);
	}
	db_delete('feedback')->condition('fid', $fid, '=')->execute();
	$form_state['redirect'] = 'admin/feedback';
}



function feedback_subj_delete_form($form, &$form_state, $fsid) {
	$form = array();
	$form['fsid'] = array(
		'#type' => 'hidden',
		'#value' => $fsid,
	);
	$confirm = confirm_form($form, t('Вы действительно хотите удалить эту запись ?'), 'admin/feedback/subj', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
	return $confirm;
}

function feedback_subj_delete_form_submit($form, &$form_state) {
	$form = array();
	$fsid = $form_state['values']['fsid'];
	
	db_delete('feedback_subj')->condition('fsid', $fsid, '=')->execute();
	
	db_query("DELETE FROM feedback WHERE subject = :fsid", array(":fsid" => $fsid));
	
	$form_state['redirect'] = 'admin/feedback/subj';
}



function feedback_settings($form, &$form_state){
	$data = variable_get('feedback_addr');
	$form['text'] = array(
		'#type' => 'text_format',
		'#title' => 'Текст',
		'#rows' => 20,
		'#format' => 'full_html',
		'#default_value' => isset($data) ? $data : '',
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Send'),
	);
	return $form;
}

function feedback_settings_submit($form, &$form_state){
	variable_set('feedback_addr', $form_state['values']['text']['value']);
	drupal_set_message('Настройки сохранены');
}
function feedback_settings_mail($form, &$form_state) {
	$path = drupal_get_path('module', 'feedback');
	/*  drupal_add_js($path . '/scripts/j.js', 'file');*/
  drupal_add_css($path . '/css/feedback.css', 'file'); 
	$form['mail'] = array(
		'#type' => 'textfield',
		'#title' => t('E-mail for Notification'),
		'#size' => 40,
		'#maxlength' => 45,
		'#default_value' => (variable_get('feedback_mail_notification')) ? variable_get('feedback_mail_notification') : '',
		'#required' => TRUE,
	);
	$form['fields'] = array(
		'#type' => 'fieldset',
		'#title' => 'Поля',
		'#collapsible' => TRUE,
		'#collapsed' => TRUE,
	);
	$form['fields']['page_title'] = array(
		'#type' => 'textfield',
		'#title' => 'Заголовок формы',
		'#default_value' => variable_get('feedback_title', 'Обратная связь'),
	);
	$form['fields']['page_text'] = array(
		'#type' => 'checkbox',
		'#title' => 'Выводить текст перед формой',
		'#default_value' => variable_get('feedback_page_text', 1),
	);
	$form['fields']['field_name'] = array(
		'#type' => 'checkbox',
		'#title' => variable_get('feedback_field_1_name', 'Имя'),
		'#default_value' => variable_get('feedback_1_enabled', 1),
		'#description' => 'Включить поле в форму',
		'#prefix' => '<div class="fields_box">',
	);
	$form['fields']['field_name_req'] = array(
		'#type' => 'checkbox',
		'#title' => 'Обязательное',
		'#default_value' => variable_get('feedback_1_required', 1),
	);
	$form['fields']['field_name_text'] = array(
		'#type' => 'textfield',
		'#title' => 'Название поля',
		'#default_value' => variable_get('feedback_1_name', 'Имя'),
		'#suffix' => '</div>',
	);
	$form['fields']['field_mail'] = array(
		'#type' => 'checkbox',
		'#title' => variable_get('feedback_field_2_name', 'E-mail'),
		'#description' => 'Включить поле в форму',
		'#default_value' => variable_get('feedback_2_enabled', 1),
		'#prefix' => '<div class="fields_box">',
	);
	$form['fields']['field_mail_req'] = array(
		'#type' => 'checkbox',
		'#title' => 'Обязательное',
		'#default_value' => variable_get('feedback_2_required', 1),
	);
	$form['fields']['field_mail_valid'] = array(
		'#type' => 'checkbox',
		'#title' => 'Валидация E-mail',
		'#default_value' => variable_get('feedback_2_valid', 1),
	);
	$form['fields']['field_mail_text'] = array(
		'#type' => 'textfield',
		'#title' => 'Название поля',
		'#default_value' => variable_get('feedback_2_name', 'E-mail'),
		'#suffix' => '</div>',
	);
	$form['fields']['field_subj'] = array(
		'#type' => 'checkbox',
		'#title' => variable_get('feedback_field_3_name', 'Тема'),
		'#default_value' => variable_get('feedback_3_enabled', 1),
		'#description' => 'Включить поле в форму',
		'#prefix' => '<div class="fields_box">',
	);
	$form['fields']['field_subj_req'] = array(
		'#type' => 'checkbox',
		'#title' => 'Обязательное',
		'#default_value' => variable_get('feedback_3_required', 1),
	);
	$form['fields']['field_subj_text'] = array(
		'#type' => 'textfield',
		'#title' => 'Название поля',
		'#default_value' => variable_get('feedback_3_name', 'Тема'),
		'#suffix' => '</div>',
	);
	$form['fields']['field_mess_text'] = array(
		'#type' => 'textfield',
		'#title' => 'Название поля ' . variable_get('feedback_4_name', 'Сообщение'),
		'#default_value' => variable_get('feedback_4_name', 'Сообщение'),
		'#prefix' => '<div class="fields_box">',
		'#suffix' => '</div>',
	);
	$form['block'] = array(
		'#type' => 'fieldset',
		'#title' => 'Блок',
		'#collapsible' => TRUE,
		'#collapsed' => TRUE,
	);
	$form['block']['block_title'] = array(
		'#type' => 'textfield',
		'#title' => 'Заголовок блока',
		'#default_value' => variable_get('feedback_block_title', 'Обратная связь'),
	);
	$form['block']['block_text'] = array(
		'#type' => 'checkbox',
		'#title' => 'Выводить текст перед формой',
		'#default_value' => variable_get('feedback_block_text', 1),
	);
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save'),
	);
	return $form;
}
function feedback_settings_mail_validate($form, &$form_state) {
	if(!valid_email_address($form_state['values']['mail'])){
		form_set_error('mail', 'Mail is invalid.');
	}
}
function feedback_settings_mail_submit($form, &$form_state) {
	variable_set('feedback_mail_notification', $form_state['values']['mail']);
	variable_set('feedback_1_enabled', $form_state['values']['field_name']);
	variable_set('feedback_2_enabled', $form_state['values']['field_mail']);
	variable_set('feedback_3_enabled', $form_state['values']['field_subj']);
	variable_set('feedback_1_name', $form_state['values']['field_name_text']);
	variable_set('feedback_2_name', $form_state['values']['field_mail_text']);
	variable_set('feedback_3_name', $form_state['values']['field_subj_text']);
	variable_set('feedback_1_required', $form_state['values']['field_name_req']);
	variable_set('feedback_2_required', $form_state['values']['field_mail_req']);
	variable_set('feedback_3_required', $form_state['values']['field_subj_req']);
	variable_set('feedback_2_valid', $form_state['values']['field_mail_valid']);
	variable_set('feedback_block_title', $form_state['values']['block_title']);
	variable_set('feedback_title', $form_state['values']['page_title']);
	variable_set('feedback_4_name', $form_state['values']['field_mess_text']);
	variable_set('feedback_page_text', $form_state['values']['page_text']);
	variable_set('feedback_block_text', $form_state['values']['block_text']);

	drupal_set_message(t('Settings have saved'));
}
